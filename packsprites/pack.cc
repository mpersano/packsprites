#include <cassert>
#include <cstring>
#include <utility>
#include <sstream>

#include <algorithm>

#include <tinyxml.h>

#include "sprite_base.h"
#include "png_util.h"
#include "panic.h"
#include "pack.h"

namespace {

struct rect
{
	rect(int left, int top, int width, int height)
	: left_(left), top_(top), width_(width), height_(height)
	{ }

	int left_, top_, width_, height_;

	std::pair<rect, rect> split_vert(int c) const;
	std::pair<rect, rect> split_horiz(int r);
};

std::pair<rect, rect>
rect::split_vert(int c) const
{
	assert(c < width_);
	return std::pair<rect, rect>(rect(left_, top_, c, height_), rect(left_ + c, top_, width_ - c, height_));
}

std::pair<rect, rect>
rect::split_horiz(int r)
{
	assert(r < height_);
	return std::pair<rect, rect>(rect(left_, top_, width_, r), rect(left_, top_ + r, width_, height_ - r));
}

struct node
{
	node(const rect& rc)
	: rc_(rc), border_(0), sprite_(0)
	{ }

	bool insert(const sprite_base *sp, int border);

	rect rc_;
	int border_;
	const sprite_base *sprite_;
	std::unique_ptr<node> left_, right_;
};

bool
node::insert(const sprite_base *sp, int border)
{
	if (left_ != NULL) {
		// not a leaf
		return left_->insert(sp, border) || right_->insert(sp, border);
	} else {
		const int wanted_width = sp->width() + 2*border;
		const int wanted_height = sp->height() + 2*border;

		// doesn't fit or already occupied
		if (sprite_ || rc_.width_ < wanted_width || rc_.height_ < wanted_height) {
			return false;
		}

		if (rc_.width_ == wanted_width && rc_.height_ == wanted_height) {
			sprite_ = sp;
			border_ = border;
			return true;
		}

		if (rc_.width_ - wanted_width > rc_.height_ - wanted_height) {
			std::pair<rect, rect> child_rect = rc_.split_vert(wanted_width);
			left_.reset(new node(child_rect.first));
			right_.reset(new node(child_rect.second));
		} else {
			std::pair<rect, rect> child_rect = rc_.split_horiz(wanted_height);
			left_.reset(new node(child_rect.first));
			right_.reset(new node(child_rect.second));
		}

		bool rv = left_->insert(sp, border);
		assert(rv);
		return rv;
	}
}

void
write_sprite_sheet(image<uint32_t>& im, const node *root)
{
	if (root->left_) {
		write_sprite_sheet(im, root->left_.get());
		write_sprite_sheet(im, root->right_.get());
	} else if (root->sprite_) {
		const auto& child_im = root->sprite_->image_;
		im.copy(*child_im, root->rc_.top_ + root->border_, root->rc_.left_ + root->border_);
	}
}

void
write_sprite_sheet(const std::string& name, const node *root)
{
	assert(root->rc_.top_ == 0 && root->rc_.left_ == 0);

	image<uint32_t> im(root->rc_.width_, root->rc_.height_);
	write_sprite_sheet(im, root);
	png_write(im, name);
}

} // (anonymous namespace)

void
pack(const std::vector<std::unique_ptr<sprite_base>>& sprites,
		const std::string& sheet_name,
		int sheet_width, int sheet_height,
		int border,
		const std::string& texture_path_base)
{
	// pack

	std::vector<const sprite_base *> sorted_sprites;
	sorted_sprites.reserve(sprites.size());

	std::transform(
		std::begin(sprites),
		std::end(sprites),
		std::back_inserter(sorted_sprites),
		[](const std::unique_ptr<sprite_base>& p) { return p.get(); });

	std::vector<std::unique_ptr<node>> trees;

	while (!sorted_sprites.empty()) {
		std::sort(
			std::begin(sorted_sprites),
			std::end(sorted_sprites),
			[](const sprite_base *a, const sprite_base *b)
			{
				return b->width()*b->height() < a->width()*a->height();
			});

		auto tree = new node { rect { 0, 0, sheet_width, sheet_height } };

		auto it = std::begin(sorted_sprites);

		while (it != std::end(sorted_sprites)) {
			if (tree->insert(*it, border))
				it = sorted_sprites.erase(it);
			else
				++it;
		}

		trees.emplace_back(tree);
	}

	auto texture_name = [&](size_t i)
		{
			std::stringstream ss;
			ss << texture_path_base << "/" << sheet_name << "." << i << ".png";
			return ss.str();
		};

	// write textures

	for (size_t i = 0; i < trees.size(); i++)
		write_sprite_sheet(texture_name(i), trees[i].get());

	// write sprite sheets

	TiXmlDocument doc;

	auto decl = new TiXmlDeclaration( "1.0", "", "" );
	doc.LinkEndChild(decl);

	auto spritesheet_node = new TiXmlElement("spritesheet");
	doc.LinkEndChild(spritesheet_node);

	auto textures_node = new TiXmlElement("textures");

	for (size_t i = 0; i < trees.size(); i++) {
		auto el = new TiXmlElement("texture");
		el->SetAttribute("path", texture_name(i));
		textures_node->LinkEndChild(el);
	}

	spritesheet_node->LinkEndChild(textures_node);

	auto sprites_node = new TiXmlElement("sprites");

	for (size_t i = 0; i < trees.size(); i++) {
		std::function<void(node *)> serialize_sprites = [&](const node *root)
			{
				if (root->left_) {
					serialize_sprites(root->left_.get());
					assert(root->right_);
					serialize_sprites(root->right_.get());
				} else if (root->sprite_) {
					auto& rc = root->rc_;
					auto border = root->border_;
					auto sp = root->sprite_;

					auto *el = new TiXmlElement("sprite");

					el->SetAttribute("x", rc.left_ + border);
					el->SetAttribute("y", rc.top_ + border);
					el->SetAttribute("w", sp->width());
					el->SetAttribute("h", sp->height());
					el->SetAttribute("tex", i);

					sp->serialize(el);

					sprites_node->LinkEndChild(el);
				}
			};

		serialize_sprites(trees[i].get());
	}

	spritesheet_node->LinkEndChild(sprites_node);

	doc.SaveFile(std::string(sheet_name) + ".spr");
}
