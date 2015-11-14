#include <cassert>
#include <cstring>
#include <utility>

#include <algorithm>

#include <tinyxml.h>

#include "panic.h"
#include "sprite_base.h"
#include "sprite_packer.h"
#include "png_util.h"

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

	bool insert_sprite(sprite_base *sp, int border);

	template <typename F>
	void for_each_sprite(F f)
	{
		if (left_) {
			left_->for_each_sprite(f);
			assert(right_);
			right_->for_each_sprite(f);
		} else if (sprite_) {
			f(rc_, border_, sprite_);
		}
	}

	rect rc_;
	int border_;
	sprite_base *sprite_;
	std::unique_ptr<node> left_, right_;
};

bool
node::insert_sprite(sprite_base *sp, int border)
{
	if (left_ != NULL) {
		// not a leaf
		return left_->insert_sprite(sp, border) || right_->insert_sprite(sp, border);
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

		bool rv = left_->insert_sprite(sp, border);
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
pack_sprites(std::vector<std::unique_ptr<sprite_base>>& sprites,
		const std::string& sheet_name,
		int sheet_width, int sheet_height,
		int border,
		const std::string& texture_path_base)
{
	const size_t num_sprites = sprites.size();

	std::sort(
		std::begin(sprites),
		std::end(sprites),
		[](const std::unique_ptr<sprite_base>& a, const std::unique_ptr<sprite_base>& b)
			{
				return b->width()*b->height() < a->width()*a->height();
			});

	std::unique_ptr<node> root { new node(rect(0, 0, sheet_width, sheet_height)) };

	for (auto& sp : sprites) {
		assert(sp->width() <= sheet_width && sp->height() <= sheet_height);

		if (!root->insert_sprite(sp.get(), border))
			panic("sprite sheet too small!\n");
	}

	// write sprite sheets

	{
	TiXmlDocument doc;

	auto *decl = new TiXmlDeclaration( "1.0", "", "" );
	doc.LinkEndChild(decl);

	auto *spritesheet = new TiXmlElement("spritesheet");
	doc.LinkEndChild(spritesheet);

	auto *texture = new TiXmlElement("texture");
	texture->SetAttribute("path", texture_path_base + "/" + sheet_name + ".png");
	spritesheet->LinkEndChild(texture);

	auto *sprites = new TiXmlElement("sprites");

	root->for_each_sprite(
		[&] (const rect& rc, int border, const sprite_base *sp)
		{
			auto *el = new TiXmlElement("sprite");

			el->SetAttribute("x", rc.left_ + border);
			el->SetAttribute("y", rc.top_ + border);
			el->SetAttribute("w", sp->width());
			el->SetAttribute("h", sp->height());

			sp->serialize(el);

			sprites->LinkEndChild(el);
		});

	spritesheet->LinkEndChild(sprites);

	doc.SaveFile(std::string(sheet_name) + ".spr");
	}

	// write texture
	write_sprite_sheet(sheet_name + ".png", root.get());
}
